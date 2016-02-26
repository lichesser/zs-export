<?hh // strict
namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;


class EcondaDE extends CSVGenerator
{
    /*
     * @var ElasticExportHelper
     */
    private ElasticExportHelper $elasticExportHelper;

    /*
     * @var ArrayHelper
     */
    private ArrayHelper $arrayHelper;

    /**
     * EcondaDE constructor.
     * @param ElasticExportHelper $elasticExportHelper
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ElasticExportHelper $elasticExportHelper, ArrayHelper $arrayHelper)
    {
        $this->elasticExportHelper = $elasticExportHelper;
        $this->arrayHelper = $arrayHelper;
    }

    /**
     * @param mixed $resultData
     */
    protected function generateContent(mixed $resultData, array<FormatSetting> $formatSettings = []):void
	{
		if($resultData instanceof RecordList)
		{
			$settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

			$this->setDelimiter(';');

			$this->addCSVContent([
					'Id',
                    'Name',
                    'Description',
                    'ProductURL',
                    'ImageURL',
                    'Price',
                    'MSRP',
                    'New',
                    'Stock',
                    'EAN',
                    'Brand',
                    'ProductCategory',
                    'Grundpreis'

			]);

            $itemCondition = [  0 => '1', // plenty condition: NEU
                                1 => '0', // plenty condition: GEBRAUCHT
                                2 => '1', // plenty condition: NEU & OVP
                                3 => '1', // plenty condition: NEU mit Etikett
                                4 => '0', // plenty condition: B-WARE
                             ];

			foreach($resultData as $item)
			{
				$data = [

                    'Id'                => $item->itemBase->id,
                    'Name'              => $this->elasticExportHelper->getName($item, $settings),
                    'Description'       => $this->elasticExportHelper->getDescription($item, $settings),
                    'ProductURL'        => $this->elasticExportHelper->getUrl($item, $settings, true, false),
                    'ImageURL'          => $this->elasticExportHelper->getMainImage($item, $settings),
                    'Price'             => number_format($this->elasticExportHelper->getPrice($item, $settings), 2, ',', ''),
                    'MSRP'              => '',
                    'New'               => $itemCondition[(int)$item->itemBase->condition],
                    'Stock'             => $item->variationStock->stockNet,
                    'EAN'               => $this->elasticExportHelper->getBarcodeByType($item, $settings, ElasticExportHelper::BARCODE_EAN),
                    'Brand'             => $item->itemBase->producer,
                    'ProductCategory'   => $this->elasticExportHelper->getCategory($item, $settings),
                    'Grundpreis'        => $this->elasticExportHelper->getBasePrice($item, $settings),
				];

				$this->addCSVContent(array_values($data));
			}
        }
    }
}