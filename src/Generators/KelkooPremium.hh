<?hh // strict
namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;


class KelkooPremium extends CSVGenerator
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
     * KelkooPremium constructor.
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

			$this->setDelimiter(" ");

			$this->addCSVContent([
					'category',
                    'marke',
                    'title',
                    'description',
                    'price',
                    'deliverycost',
                    'url',
                    'image',
                    'availability',
                    'offerid',
                    'unitaryPrice',
                    'ean',

			]);

			foreach($resultData as $item)
			{
				$data = [
                    'category'      => $this->elasticExportHelper->getCategory($item, $settings),
                    'marke'         => $item->itemBase->producer,
                    'title' 		=> $this->elasticExportHelper->getName($item, $settings),
                    'description'   => $this->elasticExportHelper->getDescription($item, $settings, 256),
                    'price' 	    => number_format($this->elasticExportHelper->getPrice($item, $settings), 2, '.', ''),
                    'deliverycost' 	=> number_format($this->elasticExportHelper->getShippingCost($item, $settings), 2, ',', ''),
                    'url' 		    => $this->elasticExportHelper->getUrl($item, $settings, true, false),
                    'image'		    => $this->elasticExportHelper->getMainImage($item, $settings),
                    'availability'  => $this->elasticExportHelper->getAvailability($item, $settings),
                    'offerid'       => $item->variationBase->id,
                    'unitaryPrice'  => $this->elasticExportHelper->getBasePrice($item, $settings),
                    'ean'           => $this->elasticExportHelper->getBarcodeByType($item, $settings, ElasticExportHelper::BARCODE_EAN),
				];

				$this->addCSVContent(array_values($data));
			}
        }
    }
}