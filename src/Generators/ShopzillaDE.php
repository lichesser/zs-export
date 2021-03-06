<?php
namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;

class ShopzillaDE extends CSVGenerator
{
	/*
     * @var ElasticExportHelper
     */
    private $elasticExportHelper;

	/*
	 * @var ArrayHelper
	 */
	private $arrayHelper;

	/**
     * Shopzilla constructor.
     * @param ElasticExportHelper $elasticExportHelper
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ElasticExportHelper $elasticExportHelper, ArrayHelper $arrayHelper)
    {
        $this->elasticExportHelper = $elasticExportHelper;
		$this->arrayHelper = $arrayHelper;
    }

    /**
     * @param RecordList $resultData
     * @param array $formatSettings
     */
    protected function generateContent($resultData, array $formatSettings = [])
	{
		if($resultData instanceof RecordList)
		{
			$settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

			$this->setDelimiter(" ");

			$this->addCSVContent([
				'Kategorie',
				'Hersteller',
				'Bezeichnung',
				'Beschreibung',
				'Artikel-URL',
				'Bild-URL',
				'SKU',
				'Bestand',
				'Versandgewicht',
				'Zustand',
				'Versandkosten',
				'Gebot',
				'Werbetext',
				'EAN',
				'Preis',
				'Grundpreis',
			]);

			foreach($resultData as $item)
			{
			    $deliveryCost = $this->elasticExportHelper->getShippingCost($item, $settings);
                if(!is_null($deliveryCost))
                {
                    $deliveryCost = number_format((float)$deliveryCost, 2, ',', '');
                }
                else
                {
                    $deliveryCost = '';
                }

				$data = [
					'Kategorie' 		=> $this->elasticExportHelper->getCategory((int)$item->variationStandardCategory->categoryId, $settings->get('lang'), $settings->get('plentyId')),
					'Hersteller' 		=> $this->elasticExportHelper->getExternalManufacturerName((int)$item->itemBase->producerId),
					'Bezeichnung' 		=> $this->elasticExportHelper->getName($item, $settings, 256),
					'Beschreibung' 		=> $this->elasticExportHelper->getDescription($item, $settings, 256),
					'Artikel-URL' 		=> $this->elasticExportHelper->getUrl($item, $settings, true, false),
					'Bild-URL' 			=> $this->elasticExportHelper->getMainImage($item, $settings),
					'SKU' 				=> $item->itemBase->id,
					'Bestand' 			=> 'Auf Lager',
					'Versandgewicht' 	=> $item->variationBase->weightG,
					'Zustand' 			=> 'Neu',
					'Versandkosten' 	=> $deliveryCost,
					'Gebot' 			=> '',
					'Werbetext' 		=> '2',
					'EAN' 				=> $item->variationBarcode->code,
					'Preis' 			=> number_format((float)$this->elasticExportHelper->getPrice($item), 2, '.', ''),
					'Grundpreis' 		=> $this->elasticExportHelper->getBasePrice($item, $settings),
				];

				$this->addCSVContent(array_values($data));
			}
		}
	}
}
